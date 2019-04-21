//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	struct SerializationContext;

	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Serialization-Internal
	 *  @{
	 */

	/**
	 * Represents an interface RTTI objects need to implement if they want to provide custom "diff" generation and applying.
	 */
	class BS_UTILITY_EXPORT IDiff
	{
	public:
		virtual ~IDiff() = default;

		/**
		 * Generates per-field differences between the provided original and new object. Any field or array entry that is 
		 * different in the new object compared to the original will be output in the resulting object, with a full 
		 * hierarchy of that field.
		 *
		 * Will return null if there is no difference.
		 */
		SPtr<SerializedObject> generateDiff(const SPtr<SerializedObject>& orgObj, const SPtr<SerializedObject>& newObj);

		/**
		 * Applies a previously generated per-field differences to the provided object. This will essentially transform the
		 * original object the differences were generated for into the modified version.
		 */
		void applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff, SerializationContext* context);

	protected:
		typedef UnorderedMap<SPtr<SerializedObject>, SPtr<SerializedObject>> ObjectMap;
		typedef UnorderedMap<SPtr<SerializedObject>, SPtr<IReflectable>> DiffObjectMap;

		/** Types of commands that are used when applying difference field values. */
		enum DiffCommandType
		{
			Diff_Plain = 0x01,
			Diff_Reflectable = 0x02,
			Diff_ReflectablePtr = 0x03,
			Diff_DataBlock = 0x04,
			Diff_ArraySize = 0x05,
			Diff_ObjectStart = 0x06,
			Diff_ObjectEnd = 0x07,
			Diff_SubObjectStart = 0x08,
			Diff_ArrayFlag = 0x10
		};
		
		/**
		 * A command that is used for delaying writing to an object, it contains all necessary information for setting RTTI 
		 * field values on an object.
		 */
		struct DiffCommand
		{
			RTTIField* field;
			UINT32 type;
			SPtr<IReflectable> object;
			UINT8* value;
			SPtr<DataStream> streamValue;
			UINT32 size;

			union
			{
				UINT32 arrayIdx;
				UINT32 arraySize;
				RTTITypeBase* rttiType;
			};
		};

		/**
		 * Recursive version of generateDiff(const SPtr<SerializedObject>&, const SPtr<SerializedObject>&).
		 *
		 * @see		generateDiff(const SPtr<SerializedObject>&, const SPtr<SerializedObject>&)
		 */
		virtual SPtr<SerializedObject> generateDiff(const SPtr<SerializedObject>& orgObj, 
			const SPtr<SerializedObject>& newObj, ObjectMap& objectMap) = 0;

		/**
		 * Generates a difference between data of a specific field type indiscriminately of the specific field type.
		 *
		 * @see		generateDiff(const SPtr<SerializedObject>&, const SPtr<SerializedObject>&)
		 */
		SPtr<SerializedInstance> generateDiff(RTTITypeBase* rtti, UINT32 fieldType, const SPtr<SerializedInstance>& orgData,
			const SPtr<SerializedInstance>& newData, ObjectMap& objectMap);

		/**
		 * Recursive version of applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff). Outputs a 
		 * set of commands that then must be executed in order to actually apply the difference to the provided object.
		 *
		 * @see		applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff)
		 */
		virtual void applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff, FrameAlloc& alloc,
			DiffObjectMap& objectMap, FrameVector<DiffCommand>& diffCommands, SerializationContext* context) = 0;

		/**
		 * Applies diff according to the diff handler retrieved from the provided RTTI object.
		 *
		 * @see		applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff)
		 */
		void applyDiff(RTTITypeBase* rtti, const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff,
			FrameAlloc& alloc, DiffObjectMap& objectMap, FrameVector<DiffCommand>& diffCommands, 
			SerializationContext* context);
	};

	/**
	 * Generates and applies "diffs". Diffs contain per-field differences between an original and new object. These 
	 * differences can be saved and then applied to an original object to transform it to the new version.
	 *
	 * @note	Objects must be in intermediate serialized format generated by IntermediateSerializer.
	 */
	class BS_UTILITY_EXPORT BinaryDiff : public IDiff
	{
	public:
		using IDiff::generateDiff;

	private:
		/** @copydoc	IDiff::generateDiff(const SPtr<SerializedObject>&, const SPtr<SerializedObject>&, ObjectMap&) */
		SPtr<SerializedObject> generateDiff(const SPtr<SerializedObject>& orgObj, const SPtr<SerializedObject>& newObj, 
			ObjectMap& objectMap) override;

		/** @copydoc	IDiff::applyDiff(const SPtr<IReflectable>&, const SPtr<SerializedObject>&, FrameAlloc&, DiffObjectMap&, FrameVector<DiffCommand>&) */
		void applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff, FrameAlloc& alloc, 
			DiffObjectMap& objectMap, FrameVector<DiffCommand>& diffCommands, SerializationContext* context) override;
	};

	/** @} */
	/** @} */
}
